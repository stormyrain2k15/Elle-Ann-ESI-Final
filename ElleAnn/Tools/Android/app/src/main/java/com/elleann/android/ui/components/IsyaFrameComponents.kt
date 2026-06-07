package com.elleann.android.ui.components

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.elleann.android.R
import com.elleann.android.ui.theme.*

data class IsyaNavItem(
    val label: String,
    val icon: ImageVector,
    val route: String,
)

@Composable
fun IsyaBottomNav(
    items: List<IsyaNavItem>,
    currentRoute: String,
    onItemSelected: (IsyaNavItem) -> Unit,
    modifier: Modifier = Modifier,
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .background(IsyaNight)
    ) {

        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(1.dp)
                .background(IsyaGold)
        )

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 1.dp),
        ) {
            items.forEach { item ->
                val active = currentRoute.startsWith(item.route)
                IsyaNavTab(
                    item     = item,
                    active   = active,
                    onClick  = { onItemSelected(item) },
                    modifier = Modifier.weight(1f),
                )
            }
        }
    }
}

@Composable
private fun IsyaNavTab(
    item: IsyaNavItem,
    active: Boolean,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val bgColor = if (active) IsyaDusk else Color.Transparent
    val iconColor = if (active) IsyaGold else IsyaMuted
    val textColor = if (active) IsyaGold else IsyaMuted

    Column(
        modifier = modifier
            .background(bgColor)
            .clickable(onClick = onClick)
            .padding(vertical = 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {

        if (active) {
            Box(
                modifier = Modifier
                    .fillMaxWidth(0.6f)
                    .height(2.dp)
                    .background(
                        Brush.horizontalGradient(
                            listOf(Color.Transparent, IsyaGoldBright, Color.Transparent)
                        )
                    )
            )
            Spacer(Modifier.height(4.dp))
        } else {
            Spacer(Modifier.height(6.dp))
        }

        Icon(
            imageVector        = item.icon,
            contentDescription = item.label,
            tint               = iconColor,
            modifier           = Modifier.size(22.dp),
        )
        Spacer(Modifier.height(3.dp))
        Text(
            text      = item.label,
            fontSize  = 10.sp,
            color     = textColor,
            fontWeight = if (active) FontWeight.Medium else FontWeight.Normal,
            textAlign = TextAlign.Center,
        )
    }
}

@Composable
fun IsyaTopBar(
    title: @Composable () -> Unit,
    modifier: Modifier = Modifier,
    navigationIcon: (@Composable () -> Unit)? = null,
    actions: (@Composable RowScope.() -> Unit)? = null,
    subtitle: (@Composable () -> Unit)? = null,
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .background(IsyaHeader)
    ) {
        Column {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 4.dp)
                    .height(if (subtitle != null) 58.dp else 44.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                navigationIcon?.invoke()

                Column(modifier = Modifier.weight(1f).padding(horizontal = 4.dp)) {
                    title()
                    subtitle?.invoke()
                }

                if (actions != null) {
                    Row(content = actions)
                }
            }

            Box(modifier = Modifier.fillMaxWidth().height(1.dp).background(IsyaGold))

            Box(modifier = Modifier.fillMaxWidth().height(1.dp).background(IsyaMagic.copy(alpha = 0.4f)))
        }
    }
}

@Composable
fun IsyaSearchBar(
    value: String,
    onValueChange: (String) -> Unit,
    placeholder: String = "Search…",
    modifier: Modifier = Modifier,
    leadingIcon: ImageVector? = null,
    trailingIcon: (@Composable () -> Unit)? = null,
) {
    androidx.compose.material3.OutlinedTextField(
        value         = value,
        onValueChange = onValueChange,
        placeholder   = {
            Text(placeholder, color = IsyaMuted, fontSize = 14.sp)
        },
        leadingIcon   = if (leadingIcon != null) {
            { Icon(leadingIcon, null, tint = IsyaMuted, modifier = Modifier.size(18.dp)) }
        } else null,
        trailingIcon  = trailingIcon,
        singleLine    = true,
        modifier      = modifier.fillMaxWidth(),
        shape         = RoundedCornerShape(8.dp),
        colors        = androidx.compose.material3.OutlinedTextFieldDefaults.colors(
            focusedTextColor        = IsyaCream,
            unfocusedTextColor      = IsyaCream,
            focusedBorderColor      = IsyaMagic,
            unfocusedBorderColor    = IsyaGold.copy(alpha = 0.5f),
            focusedContainerColor   = IsyaInput,
            unfocusedContainerColor = IsyaInput,
            cursorColor             = IsyaMagicBright,
        ),
    )
}

@Composable
fun IsyaScreenFrame(
    modifier: Modifier = Modifier,
    content: @Composable BoxScope.() -> Unit,
) {
    Box(
        modifier = modifier
            .fillMaxSize()
            .background(IsyaNight),
    ) {
        content()

        val cornerSize = 28.dp

        Image(
            painter = painterResource(R.drawable.isya_corner_0),
            contentDescription = null,
            modifier = Modifier.size(cornerSize).align(Alignment.TopStart),
        )

        Image(
            painter = painterResource(R.drawable.isya_corner_1),
            contentDescription = null,
            modifier = Modifier.size(cornerSize).align(Alignment.TopEnd),
        )

        Image(
            painter = painterResource(R.drawable.isya_corner_3),
            contentDescription = null,
            modifier = Modifier.size(cornerSize).align(Alignment.BottomStart),
        )

        Image(
            painter = painterResource(R.drawable.isya_corner_2),
            contentDescription = null,
            modifier = Modifier.size(cornerSize).align(Alignment.BottomEnd),
        )
    }
}

@Composable
fun IsyaTabStrip(
    tabs: List<String>,
    selectedIndex: Int,
    onTabSelected: (Int) -> Unit,
    modifier: Modifier = Modifier,
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .background(IsyaNight)
            .padding(horizontal = 12.dp, vertical = 4.dp),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
    ) {
        tabs.forEachIndexed { i, label ->
            val active = i == selectedIndex
            Box(
                modifier = Modifier
                    .weight(1f)
                    .clip(RoundedCornerShape(6.dp))
                    .background(if (active) IsyaDusk else Color.Transparent)
                    .border(
                        width = if (active) 1.5.dp else 1.dp,
                        color = if (active) IsyaGold else IsyaMist,
                        shape = RoundedCornerShape(6.dp),
                    )
                    .clickable { onTabSelected(i) }
                    .padding(vertical = 7.dp),
                contentAlignment = Alignment.Center,
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text      = label,
                        fontSize  = 12.sp,
                        color     = if (active) IsyaGold else IsyaMuted,
                        fontWeight = if (active) FontWeight.SemiBold else FontWeight.Normal,
                    )
                    if (active) {
                        Spacer(Modifier.height(2.dp))
                        Box(
                            modifier = Modifier
                                .width(24.dp)
                                .height(2.dp)
                                .background(IsyaGoldBright, RoundedCornerShape(1.dp))
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun IsyaHudRow(
    modifier: Modifier = Modifier,
    content: @Composable RowScope.() -> Unit,
) {
    Row(
        modifier          = modifier
            .fillMaxWidth()
            .background(IsyaMist.copy(alpha = 0.4f))
            .padding(horizontal = 12.dp, vertical = 3.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(16.dp),
        content           = content,
    )
}

@Composable
fun IsyaHudStat(label: String, value: String, valueColor: Color = IsyaCream) {
    Row(horizontalArrangement = Arrangement.spacedBy(4.dp), verticalAlignment = Alignment.CenterVertically) {
        Text(label, fontSize = 10.sp, color = IsyaMuted)
        Text(value, fontSize = 10.sp, color = valueColor, fontWeight = FontWeight.Medium)
    }
}
